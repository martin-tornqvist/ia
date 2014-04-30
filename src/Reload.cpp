#include "Reload.h"

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

namespace Reload {

namespace {

void printMsgAndPlaySfx(Actor& actorReloading, Weapon* const wpn,
                        Item* const ammo, const ReloadResult result,
                        const bool IS_SWIFT_RELOAD) {
  string ammoName = "";
  bool isClip = false;

  if(ammo != NULL) {
    ammoName = ItemData::getItemRef(*ammo, ItemRefType::a);
    isClip = ammo->getData().isAmmoClip;
  }

  const bool IS_PLAYER    = &actorReloading == Map::player;
  const string actorName  = actorReloading.getNameThe();

  switch(result) {
    case ReloadResult::notCarryingWpn: {
      if(IS_PLAYER) {
        Log::addMsg("I am not wielding a weapon.");
      }
    } break;

    case ReloadResult::wpnNotUsingAmmo: {
      if(IS_PLAYER) {
        Log::addMsg("This weapon does not use ammo.");
      }
    } break;

    case ReloadResult::alreadyFull: {
      if(IS_PLAYER) {
        Log::addMsg("Weapon already loaded.");
      }
    } break;

    case ReloadResult::noAmmo: {
      if(IS_PLAYER) {
        Log::addMsg("I carry no ammunition for this weapon.");
      }
    } break;

    case ReloadResult::success: {
      const string swiftStr = IS_SWIFT_RELOAD ? " swiftly" : "";
      if(IS_PLAYER) {

        Audio::play(wpn->getData().reloadSfx);

        if(isClip) {
          const string wpnName =
            ItemData::getItemRef(*wpn, ItemRefType::plain, true);
          Log::addMsg(
            "I" + swiftStr + " reload the " + wpnName +
            " (" + toStr(wpn->nrAmmoLoaded) + "/" +
            toStr(wpn->ammoCapacity) + ").");
        } else {
          Log::addMsg(
            "I" + swiftStr + " load " + ammoName +
            " (" + toStr(wpn->nrAmmoLoaded) + "/" +
            toStr(wpn->ammoCapacity) + ").");
        }
        Renderer::drawMapAndInterface();
      } else {
        if(Map::player->isSeeingActor(actorReloading, NULL)) {
          Log::addMsg(actorName + swiftStr + " reloads.");
        }
      }
    } break;

    case ReloadResult::fumble: {
      if(IS_PLAYER) {
        Log::addMsg("I fumble with " + ammoName + ".");
      } else {
        if(Map::player->isSeeingActor(actorReloading, NULL)) {
          Log::addMsg(actorName + " fumbles with " + ammoName + ".");
        }
      }
    } break;
  }
}

} //namespace

bool reloadWieldedWpn(Actor& actorReloading) {
  bool didAct = false;

  Inventory& inv      = actorReloading.getInv();
  Item* const wpnItem = inv.getItemInSlot(SlotId::wielded);

  if(wpnItem == NULL) {
    printMsgAndPlaySfx(actorReloading, NULL, NULL,
                       ReloadResult::notCarryingWpn, false);
    return didAct;
  }

  Weapon* const wpn     = dynamic_cast<Weapon*>(wpnItem);
  ReloadResult result = ReloadResult::noAmmo;
  bool isSwiftReload    = false;

  if(&actorReloading == Map::player) {
    isSwiftReload =
      PlayerBon::hasTrait(Trait::expertMarksman) && Rnd::coinToss();
  }

  const int wpnAmmoCapacity = wpn->ammoCapacity;

  if(wpnAmmoCapacity == 0) {
    printMsgAndPlaySfx(actorReloading, wpn, NULL,
                       ReloadResult::wpnNotUsingAmmo, false);
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
            result = ReloadResult::fumble;
            printMsgAndPlaySfx(actorReloading, NULL, item,
                               ReloadResult::fumble, false);
          } else {
            result = ReloadResult::success;
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
                item = ItemFactory::spawnItem(ammoType);
                clipItem = dynamic_cast<ItemAmmoClip*>(item);
                clipItem->ammo = previousAmmoCount;
                inv.putInGeneral(clipItem);
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
      if(result == ReloadResult::noAmmo) {
        printMsgAndPlaySfx(actorReloading, wpn, item, result,
                           isSwiftReload);
      }
    } else {
      result = ReloadResult::alreadyFull;
      printMsgAndPlaySfx(actorReloading, wpn, item, result,
                         isSwiftReload);
    }
  }

  if(result == ReloadResult::success || result == ReloadResult::fumble) {
    didAct = true;
    GameTime::actorDidAct(isSwiftReload);
  }

  return didAct;
}

} //Reload
