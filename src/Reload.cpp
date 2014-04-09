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
#include "PlayerBon.h"
#include "GameTime.h"
#include "Audio.h"
#include "Renderer.h"

void Reload::printMsgAndPlaySfx(Actor& actorReloading, Weapon* const wpn,
                                Item* const ammo, const ReloadResult result,
                                const bool IS_SWIFT_RELOAD) {
  string ammoName = "";
  bool isClip = false;

  if(ammo != NULL) {
    ammoName = eng.itemDataHandler->getItemRef(*ammo, ItemRefType::a);
    isClip = ammo->getData().isAmmoClip;
  }

  const bool IS_PLAYER    = &actorReloading == eng.player;
  const string actorName  = actorReloading.getNameThe();

  switch(result) {
    case reloadResult_notCarryingWpn: {
      if(IS_PLAYER) {
        eng.log->addMsg("I am not wielding a weapon.");
      }
    } break;

    case reloadResult_wpnNotUsingAmmo: {
      if(IS_PLAYER) {
        eng.log->addMsg("This weapon does not use ammo.");
      }
    } break;

    case reloadResult_alreadyFull: {
      if(IS_PLAYER) {
        eng.log->addMsg("Weapon already loaded.");
      }
    } break;

    case reloadResult_noAmmo: {
      if(IS_PLAYER) {
        eng.log->addMsg("I carry no ammunition for this weapon.");
      }
    } break;

    case reloadResult_success: {
      const string swiftStr = IS_SWIFT_RELOAD ? " swiftly" : "";
      if(IS_PLAYER) {

        Audio::play(wpn->getData().reloadSfx);

        if(isClip) {
          const string wpnName =
            eng.itemDataHandler->getItemRef(*wpn, ItemRefType::plain, true);
          eng.log->addMsg(
            "I" + swiftStr + " reload the " + wpnName +
            " (" + toStr(wpn->nrAmmoLoaded) + "/" +
            toStr(wpn->ammoCapacity) + ").");
        } else {
          eng.log->addMsg(
            "I" + swiftStr + " load " + ammoName +
            " (" + toStr(wpn->nrAmmoLoaded) + "/" +
            toStr(wpn->ammoCapacity) + ").");
        }
        Renderer::drawMapAndInterface();
      } else {
        if(eng.player->isSeeingActor(actorReloading, NULL)) {
          eng.log->addMsg(actorName + swiftStr + " reloads.");
        }
      }
    } break;

    case reloadResult_fumble: {
      if(IS_PLAYER) {
        eng.log->addMsg("I fumble with " + ammoName + ".");
      } else {
        if(eng.player->isSeeingActor(actorReloading, NULL)) {
          eng.log->addMsg(actorName + " fumbles with " + ammoName + ".");
        }
      }
    } break;
  }
}

bool Reload::reloadWieldedWpn(Actor& actorReloading) {
  bool didAct = false;

  Inventory& inv      = actorReloading.getInv();
  Item* const wpnItem = inv.getItemInSlot(slot_wielded);

  if(wpnItem == NULL) {
    printMsgAndPlaySfx(actorReloading, NULL, NULL,
                       reloadResult_notCarryingWpn, false);
    return didAct;
  }

  Weapon* const wpn     = dynamic_cast<Weapon*>(wpnItem);
  ReloadResult result = reloadResult_noAmmo;
  bool isSwiftReload    = false;

  if(&actorReloading == eng.player) {
    isSwiftReload = PlayerBon::hasTrait(Trait::expertMarksman) &&
                    Rnd::coinToss();
  }

  const int wpnAmmoCapacity = wpn->ammoCapacity;

  if(wpnAmmoCapacity == 0) {
    printMsgAndPlaySfx(actorReloading, wpn, NULL,
                       reloadResult_wpnNotUsingAmmo, false);
  } else {
    const ItemId ammoType = wpn->getData().rangedAmmoTypeUsed;
    Item* item = NULL;

    if(wpn->nrAmmoLoaded < wpnAmmoCapacity) {

      const vector<Item*>& general = inv.getGeneral();
      const int NR_ITEMS = general.size();
      for(int i = 0; i < NR_ITEMS; i++) {
        item = general.at(i);

        if(item->getData().id == ammoType) {
          PropHandler& propHlr = actorReloading.getPropHandler();

          vector<PropId> props;
          propHlr.getAllActivePropIds(props);

          const bool IS_RELOADER_BLIND =
            actorReloading.getPropHandler().allowSee() == false;
          const bool IS_REALOADER_TERRIFIED =
            find(props.begin(), props.end(), propTerrified) != props.end();
          const int CHANCE_TO_FUMBLE =
            (IS_RELOADER_BLIND ? 48 : 0) + (IS_REALOADER_TERRIFIED ? 48 : 0);

          if(Rnd::percentile() < CHANCE_TO_FUMBLE) {
            isSwiftReload = false;
            result = reloadResult_fumble;
            printMsgAndPlaySfx(actorReloading, NULL, item,
                               reloadResult_fumble, false);
          } else {
            result = reloadResult_success;
            bool isClip = item->getData().isAmmoClip;

            //If ammo comes in clips
            if(isClip) {
              const int previousAmmoCount = wpn->nrAmmoLoaded;
              ItemAmmoClip* clipItem = dynamic_cast<ItemAmmoClip*>(item);
              wpn->nrAmmoLoaded = clipItem->ammo;

              printMsgAndPlaySfx(actorReloading, wpn, item, result,
                                 isSwiftReload);

              //Erase loaded clip
              inv.deleteItemInGeneralWithElement(i);

              //If weapon previously contained ammo, create a new clip item
              if(previousAmmoCount > 0) {
                item = eng.itemFactory->spawnItem(ammoType);
                clipItem = dynamic_cast<ItemAmmoClip*>(item);
                clipItem->ammo = previousAmmoCount;
                inv.putItemInGeneral(clipItem);
              }
            }
            //Else ammo is a pile
            else {
              wpn->nrAmmoLoaded += 1;

              printMsgAndPlaySfx(
                actorReloading, wpn, item, result, isSwiftReload);

              //Decrease ammo item number
              inv.decrItemInGeneral(i);
            }
          }
          break;
        }
      }
      if(result == reloadResult_noAmmo) {
        printMsgAndPlaySfx(actorReloading, wpn, item, result,
                           isSwiftReload);
      }
    } else {
      result = reloadResult_alreadyFull;
      printMsgAndPlaySfx(actorReloading, wpn, item, result,
                         isSwiftReload);
    }
  }

  if(result == reloadResult_success || result == reloadResult_fumble) {
    didAct = true;
    eng.gameTime->actorDidAct(isSwiftReload);
  }

  return didAct;
}

